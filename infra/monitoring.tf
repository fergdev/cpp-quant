resource "kubernetes_namespace" "monitoring" {
  metadata { name = var.monitoring_namespace }
}

resource "helm_release" "kps" {
  name       = "kube-prometheus-stack"
  namespace  = kubernetes_namespace.monitoring.metadata[0].name
  repository = "https://prometheus-community.github.io/helm-charts"
  chart      = "kube-prometheus-stack"
  version    = "62.7.0"

  timeout         = 900
  wait            = true
  cleanup_on_fail = true
  force_update    = true

  values = [yamlencode({
    crds = { enabled = true }

    grafana = {
      adminPassword = var.grafana_admin_password
      ingress = {
        enabled          = true
        ingressClassName = "nginx"
        annotations = {
          "cert-manager.io/cluster-issuer"                 = var.cluster_issuer
          "nginx.ingress.kubernetes.io/force-ssl-redirect" = "true"
        }
        hosts = [var.grafana_host]
        tls   = [{ secretName = "grafana-cert", hosts = [var.grafana_host] }]
      }
      resources = {
        requests = { cpu = "50m", memory = "256Mi" }
        limits   = { cpu = "500m", memory = "512Mi" }
      }
      additionalDataSources = [
        {
          name      = "Loki"
          type      = "loki"
          url       = "http://loki:3100"
          access    = "proxy"
          isDefault = false
          jsonData  = { maxLines = 1000 }
        }
      ]
    }

    prometheus = {
      prometheusSpec = {
        retention = "7d"
        resources = {
          requests = { cpu = "200m", memory = "512Mi" }
          limits   = { cpu = "800m", memory = "1Gi" }
        }
      }
    }

    alertmanager = {
      alertmanagerSpec = {
        resources = {
          requests = { cpu = "50m", memory = "128Mi" }
          limits   = { cpu = "250m", memory = "256Mi" }
        }
      }
    }
  })]

  depends_on = [kubernetes_namespace.monitoring]
}

resource "helm_release" "loki" {
  name       = "loki"
  namespace  = kubernetes_namespace.monitoring.metadata[0].name
  repository = "https://grafana.github.io/helm-charts"
  chart      = "loki"
  version    = "6.6.4"

  timeout         = 900
  wait            = true
  cleanup_on_fail = true
  force_update    = true
  recreate_pods   = true

  values = [
    yamlencode({
      deploymentMode = "SingleBinary"

      loki = {
        auth_enabled = false
        commonConfig = { replication_factor = 1 }

        storage = {
          type = "filesystem"
          filesystem = {
            chunks_directory = "/var/loki/chunks"
            rules_directory  = "/var/loki/rules"
          }
        }

        schemaConfig = {
          configs = [{
            from         = "2025-01-01"
            store        = "boltdb-shipper"
            object_store = "filesystem"
            schema       = "v13"
            index        = { prefix = "loki_index_", period = "24h" }
          }]
        }
        limits_config = {
          allow_structured_metadata = false
        }
      }

      chunksCache  = { enabled = false }
      resultsCache = { enabled = false }
      gateway      = { enabled = false }
      ruler        = { enabled = false }
      test         = { enabled = false }
      read         = { replicas = 0 }
      write        = { replicas = 0 }
      backend      = { replicas = 0 }

      singleBinary = {
        resources = {
          requests = { cpu = "100m", memory = "256Mi" }
          limits   = { cpu = "500m", memory = "512Mi" }
        }
      }
    })
  ]

  depends_on = [kubernetes_namespace.monitoring]
}

resource "helm_release" "promtail" {
  name       = "promtail"
  namespace  = kubernetes_namespace.monitoring.metadata[0].name
  repository = "https://grafana.github.io/helm-charts"
  chart      = "promtail"
  version    = "6.16.6"

  values = [
    yamlencode({
      config = {
        clients = [
          { url = "http://loki:3100/loki/api/v1/push" }
        ]
      }
      resources = { requests = { cpu = "50m", memory = "128Mi" } }
    })
  ]

  depends_on = [helm_release.loki]
}

resource "helm_release" "otelcol" {
  name       = "otelcol"
  namespace  = "monitoring"
  repository = "https://open-telemetry.github.io/opentelemetry-helm-charts"
  chart      = "opentelemetry-collector"
  version    = "0.98.1"

  values = [file("${path.module}/otel-values.yaml")]

  set = [
    { name = "daemonset.securityContext.runAsUser", value = "0" },
    { name = "daemonset.extraVolumes[0].name", value = "varlog" },
    { name = "daemonset.extraVolumes[0].hostPath.path", value = "/var/log" },
    { name = "daemonset.extraVolumeMounts[0].name", value = "varlog" },
    { name = "daemonset.extraVolumeMounts[0].mountPath", value = "/var/log" },
  ]
}
