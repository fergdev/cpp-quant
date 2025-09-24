resource "kubernetes_namespace" "app" {
  metadata { name = var.namespace }
}

locals {
  dockerconfigjson = jsonencode({
    auths = {
      "${var.registry_host}" = {
        auth = base64encode("oauth2accesstoken:${var.do_token}")
      }
    }
  })
}

resource "kubernetes_secret" "docr_pull" {
  metadata {
    name      = "do-registry"
    namespace = var.namespace
  }
  type = "kubernetes.io/dockerconfigjson"

  data = {
    ".dockerconfigjson" = local.dockerconfigjson
  }
}

resource "helm_release" "ingress_nginx" {
  name             = "ingress-nginx"
  repository       = "https://kubernetes.github.io/ingress-nginx"
  chart            = "ingress-nginx"
  namespace        = "ingress-nginx"
  create_namespace = true
}

resource "helm_release" "cert_manager" {
  name             = "cert-manager"
  repository       = "https://charts.jetstack.io"
  chart            = "cert-manager"
  namespace        = "cert-manager"
  create_namespace = true
  set {
    name  = "installCRDs"
    value = "true"
  }
}

resource "kubernetes_manifest" "cluster_issuer" {
  manifest = {
    apiVersion = "cert-manager.io/v1"
    kind       = "ClusterIssuer"
    metadata   = { name = "letsencrypt" }
    spec = {
      acme = {
        email               = var.letsencrypt_email
        server              = "https://acme-v02.api.letsencrypt.org/directory"
        privateKeySecretRef = { name = "letsencrypt-account-key" }
        solvers = [{
          http01 = { ingress = { class = "nginx" } }
        }]
      }
    }
  }
  depends_on = [helm_release.cert_manager]
}
