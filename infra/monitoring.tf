resource "kubernetes_namespace" "monitoring" {
  metadata { name = var.monitoring_namespace }
}

resource "helm_release" "kps" {
  name       = "kube-prometheus-stack"
  namespace  = kubernetes_namespace.monitoring.metadata[0].name
  repository = "https://prometheus-community.github.io/helm-charts"
  chart      = "kube-prometheus-stack"
  version    = "62.7.0"

  values = [
    yamlencode({
      grafana = {
        adminPassword = var.grafana_admin_password
        ingress = {
          enabled          = true
          ingressClassName = "nginx"
          annotations = {
            "cert-manager.io/cluster-issuer"                 = var.cluster_issuer
            "nginx.ingress.kubernetes.io/force-ssl-redirect" = "true"
            "nginx.ingress.kubernetes.io/proxy-read-timeout" = "3600"
            "nginx.ingress.kubernetes.io/proxy-send-timeout" = "3600"
          }
          hosts = [var.grafana_host]
          tls = [{
            secretName = "grafana-cert"
            hosts      = [var.grafana_host]
          }]
        }
        persistence = { enabled = false }
      }

      prometheus = {
        prometheusSpec = {
          retention = "15d"
          resources = { requests = { cpu = "200m", memory = "1Gi" } }
        }
      }

      alertmanager = {
        alertmanagerSpec = {
          resources = { requests = { cpu = "100m", memory = "256Mi" } }
        }
      }
    })
  ]

  depends_on = [kubernetes_namespace.monitoring]
}

resource "helm_release" "loki" {
  name       = "loki"
  namespace  = kubernetes_namespace.monitoring.metadata[0].name
  repository = "https://grafana.github.io/helm-charts"
  chart      = "loki"
  version    = "6.6.4"

  values = [
    yamlencode({
      deploymentMode = "SingleBinary"
      persistence    = { enabled = false }
      resources      = { requests = { cpu = "100m", memory = "256Mi" } }

      gateway = {
        enabled = length(var.loki_host) > 0 ? true : false
        ingress = {
          enabled          = length(var.loki_host) > 0 ? true : false
          ingressClassName = "nginx"
          annotations = {
            "cert-manager.io/cluster-issuer" = var.cluster_issuer
          }
          hosts = [var.loki_host]
          tls = [{
            secretName = "loki-cert"
            hosts      = [var.loki_host]
          }]
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
